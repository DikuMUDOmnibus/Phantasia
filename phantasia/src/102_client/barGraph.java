import java.awt.*;

public class barGraph extends Canvas implements constants {

    float percent = 0;
    float otherPercent = 0;
    long otherMax = 0;
    String theValue;
    int cx = 0;
    int canvasWidth = 0;
    int canvasHeight = 0;
    int theFill = 0;
    Rectangle theBounds = null;
    Color fillColor = null;
    FontMetrics fm = null;
    Font theFont = new Font("Helvetica", Font.BOLD, 12);

    public barGraph() {

	super();
	setBackground(highlightColor);
	percent = 0;
	theValue = "0";
    }

    public void paint(Graphics g) {

	theBounds = bounds();
	canvasWidth = theBounds.width - 1;
	canvasHeight = theBounds.height - 1;

	if (percent == 1) {
	    fillColor = Color.green;
	}
	else if (percent < .2) {
	    fillColor = Color.red;
	}
	else if (percent > .7) {
	    fillColor = new Color((float)Math.cos(3.14159*(percent-.7)), (float)1.0, (float)0);
	}
	else {
	    fillColor = new Color((float)1.0, (float)Math.sin(3.14159*(percent-.2)), (float)0);
	}

	fm = getFontMetrics(theFont);
	cx = (int) ((canvasWidth - fm.stringWidth(theValue)) / 2);

	g.setColor(highlightColor);
	g.clearRect(0, 0, canvasWidth, canvasHeight);

	if (otherPercent > 0) {
	    g.setColor(Color.blue);
	    theFill = (int) (otherPercent * canvasWidth);
	    g.fillRect(0, 0, theFill, canvasHeight);
	}

	g.setColor(foregroundColor);
	g.drawRect(0, 0, canvasWidth, canvasHeight);
	g.drawRect(1, 1, canvasWidth - 2, canvasHeight - 2);
	
	g.setColor(fillColor);
	theFill = (int) (percent * (canvasWidth - 7));
	g.fillRect(4, 4, theFill, canvasHeight - 7);

	g.setColor(foregroundColor);
	g.setFont(theFont);
	g.drawString(theValue, cx, 14);
    }

    public void changeStats(String value, long maxValue, long secondValue) {

	percent = (float)Long.parseLong(value)/maxValue;
	theValue = value;

	if (secondValue == 0) {
	    otherPercent = 0;
	}
	else if (secondValue > otherMax) {
	    otherMax = secondValue;
	    otherPercent = 1;
	}
	else {
	    otherPercent = (float)secondValue/otherMax;
	}

	paint(getGraphics());
    }
}
