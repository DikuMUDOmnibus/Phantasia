import java.awt.*;

public class numberPne extends Canvas implements constants {

    String theValue;
    int canvasWidth = 0;
    int canvasHeight = 0;
    Rectangle theBounds = null;
    Font theFont = new Font("Helvetica", Font.BOLD, 12);

    public numberPne(String message) {

	super();
	theValue = message;
	setBackground(highlightColor);
	setSize(100, 18);
    }

    public void paint(Graphics g) {

	theBounds = getBounds();
	canvasWidth = theBounds.width - 1;
	canvasHeight = theBounds.height - 1;

	g.setColor(highlightColor);
	g.clearRect(0, 0, canvasWidth, canvasHeight);

	g.setColor(foregroundColor);
	g.setFont(theFont);
	g.drawString(theValue, 0, canvasHeight - 3);
    }

    public void changeStats(String message) {

	theValue = message;
	paint(getGraphics());
    }
}
