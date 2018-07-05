import java.awt.*;

public class titlePne extends Canvas implements constants {

    String theValue;
    int cx = 0;
    int canvasWidth = 0;
    int canvasHeight = 0;
    Rectangle theBounds = null;
    FontMetrics fm = null;
    Font theFont = new Font("Helvetica", Font.BOLD, 12);

    public titlePne(String message) {

	super();
	theValue = message;
	setBackground(highlightColor);
    }

    public void paint(Graphics g) {

	theBounds = bounds();
	canvasWidth = theBounds.width - 1;
	canvasHeight = theBounds.height - 1;

	fm = getFontMetrics(theFont);
	cx = (int) ((canvasWidth - fm.stringWidth(theValue)) / 2);

	g.setColor(highlightColor);
	g.clearRect(0, 0, canvasWidth, canvasHeight);

	g.setColor(foregroundColor);
	g.drawLine(0, canvasHeight - 3, canvasWidth, canvasHeight - 3);
	g.setFont(theFont);
	g.drawString(theValue, cx, canvasHeight - 4);
    }
}
